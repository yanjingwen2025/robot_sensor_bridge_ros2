#include <chrono>
#include <cmath>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int64.hpp"
#include "sensor_msgs/msg/imu.hpp"

using namespace std::chrono_literals;

class ImuTestPublisher : public rclcpp::Node {
public:
    ImuTestPublisher() : Node("imu_test_publisher"), publish_count_(0) {
        // ===== 1. 声明参数 =====
        this->declare_parameter<double>("frequency", 100.0);
        this->declare_parameter<bool>("use_reliable", true);
        this->declare_parameter<std::string>("topic", "/imu/data");
        this->declare_parameter<std::string>("frame_id", "imu_link");

        double frequency = this->get_parameter("frequency").as_double();
        bool use_reliable = this->get_parameter("use_reliable").as_bool();
        std::string topic = this->get_parameter("topic").as_string();
        frame_id_ = this->get_parameter("frame_id").as_string();

        if (frequency <= 0.0) {
            RCLCPP_ERROR(this->get_logger(), "frequency must be > 0, got %.2f", frequency);
            throw std::runtime_error("invalid frequency");
        }

        // ===== 2. QoS 配置 =====
        rclcpp::QoS qos(10);
        if (use_reliable) {
            qos.reliable();
        } else {
            qos.best_effort();
        }

        // ===== 3. 创建发布者 =====
        publisher_ = this->create_publisher<sensor_msgs::msg::Imu>(topic, qos);
        stats_publisher_ = this->create_publisher<std_msgs::msg::UInt64>(
    "/publisher_stats", 10);

        // ===== 4. 定时器：1/frequency 秒发一条 =====
        auto period_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(1.0 / frequency));
        timer_ = this->create_wall_timer(period_ns,
            std::bind(&ImuTestPublisher::publish_imu, this));

        // ===== 5. 统计定时器：每 1 秒打印一次 =====
        stats_timer_ = this->create_wall_timer(
            1s, std::bind(&ImuTestPublisher::print_stats, this));

        RCLCPP_INFO(this->get_logger(),
                    "IMU Test Publisher started. topic=%s, frequency=%.2f Hz, QoS=%s",
                    topic.c_str(), frequency, use_reliable ? "RELIABLE" : "BEST_EFFORT");
    }

private:
    void publish_imu() {
        auto msg = sensor_msgs::msg::Imu();
        msg.header.stamp = this->now();
        msg.header.frame_id = frame_id_;

        // 造点正弦波数据，看起来像真 IMU
        double t = this->now().seconds();
        msg.angular_velocity.x = std::sin(t);
        msg.angular_velocity.y = std::cos(t);
        msg.angular_velocity.z = 0.0;

        msg.linear_acceleration.x = 0.0;
        msg.linear_acceleration.y = 0.0;
        msg.linear_acceleration.z = 9.81;

        msg.orientation_covariance[0] = -1.0;
        msg.angular_velocity_covariance[0] = -1.0;
        msg.linear_acceleration_covariance[0] = -1.0;

        publisher_->publish(msg);
        publish_count_++;
    }

    void print_stats() {
    auto msg = std_msgs::msg::UInt64();
    msg.data = publish_count_;
    stats_publisher_->publish(msg);                     //  发 topic

    // 保留日志
    RCLCPP_INFO(this->get_logger(), "PUBSTATS|%lu",
                static_cast<unsigned long>(publish_count_));
}
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
    rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr stats_publisher_;  
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::TimerBase::SharedPtr stats_timer_;

    uint64_t publish_count_;
    std::string frame_id_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ImuTestPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}