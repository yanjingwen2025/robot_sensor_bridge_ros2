#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_gateway/msg/gateway_stats.hpp"
#include "sensor_gateway/srv/get_sensor_stats.hpp"

using namespace std::chrono_literals;

class SensorGatewayNode : public rclcpp::Node {
public:
    SensorGatewayNode() : Node("sensor_gateway_node"), msg_count_(0), last_time_(0.0) {
    // ===== 1. 声明参数 =====
    this->declare_parameter<std::string>("imu_topic", "/imu/data");
    this->declare_parameter<std::string>("summary_topic", "/sensor_summary");
    this->declare_parameter<double>("publish_period", 2.0);
    // 参数声明
    this->declare_parameter<bool>("imu_use_reliable", true);
    bool imu_use_reliable = this->get_parameter("imu_use_reliable").as_bool();

    // QoS
    rclcpp::QoS qos(10);
    if (imu_use_reliable) qos.reliable();
    else qos.best_effort();


    // ===== 2. 读取参数 =====
    std::string imu_topic = this->get_parameter("imu_topic").as_string();
    std::string summary_topic = this->get_parameter("summary_topic").as_string();
    double publish_period = this->get_parameter("publish_period").as_double();

    // ===== 3. 用参数创建订阅/发布 =====
    subscription_ = this->create_subscription<sensor_msgs::msg::Imu>(
        imu_topic, qos,
        std::bind(&SensorGatewayNode::imu_callback, this, std::placeholders::_1));
        //默认QoS
    publisher_ = this->create_publisher<std_msgs::msg::String>(summary_topic, 10);
    
    stats_publisher_ = this->create_publisher<sensor_gateway::msg::GatewayStats>(
    "/gateway_stats", 10);

    // 服务名也可以参数化（可选，先保持不变）
    service_ = this->create_service<sensor_gateway::srv::GetSensorStats>(
        "get_sensor_stats",
        std::bind(&SensorGatewayNode::service_callback, this,
                  std::placeholders::_1, std::placeholders::_2));

    // ===== 4. 定时器用参数周期 =====
    timer_ = this->create_wall_timer(
        std::chrono::duration<double>(publish_period),
        std::bind(&SensorGatewayNode::publish_summary, this));

    latency_timer_ = this->create_wall_timer(
    1s, std::bind(&SensorGatewayNode::print_latency_stats, this));

    RCLCPP_INFO(this->get_logger(),
                "Sensor Gateway Node started. imu_topic=%s, period=%.2fs",
                imu_topic.c_str(), publish_period);
}

private:
    void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
        msg_count_++;

        last_time_ =
            msg->header.stamp.sec +
            msg->header.stamp.nanosec * 1e-9;

        auto now = this->now();
        double latency = (now - msg->header.stamp).seconds();

        latency_sum_ += latency;
        latency_max_ = std::max(latency_max_, latency);
        latency_count_++;

        std::this_thread::sleep_for(
            std::chrono::microseconds(2000));
    }

    void publish_summary() {
        auto message = std_msgs::msg::String();
        message.data = "Total IMU messages: " + std::to_string(msg_count_) +
                       ", last timestamp: " + std::to_string(last_time_);
        publisher_->publish(message);
        RCLCPP_INFO(this->get_logger(), "Published summary: %s", message.data.c_str());
    }

    void service_callback(
        const std::shared_ptr<sensor_gateway::srv::GetSensorStats::Request> request,
        std::shared_ptr<sensor_gateway::srv::GetSensorStats::Response> response) {
        (void)request;
        response->total_messages = msg_count_;
        response->last_timestamp = last_time_;
        response->status = "running";
        RCLCPP_INFO(this->get_logger(), "Service called: returning stats");
    }

    void print_latency_stats()
    {
        if (latency_count_ > 0) {
            auto stats_msg =
                sensor_gateway::msg::GatewayStats();

            stats_msg.total_messages = msg_count_;

            stats_msg.avg_latency =
                latency_sum_ / latency_count_;

            stats_msg.max_latency =
                latency_max_;

            stats_publisher_->publish(stats_msg);

            RCLCPP_INFO(
                this->get_logger(),
                "STATS|%lu|%.6f|%.6f",
                static_cast<unsigned long>(msg_count_),
                stats_msg.avg_latency,
                stats_msg.max_latency);

            latency_sum_ = 0.0;
            latency_max_ = 0.0;
            latency_count_ = 0;
        }
    }

    // ROS2资源句柄
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::Publisher<sensor_gateway::msg::GatewayStats>::SharedPtr stats_publisher_;
    rclcpp::Service<sensor_gateway::srv::GetSensorStats>::SharedPtr service_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::TimerBase::SharedPtr latency_timer_;

    uint32_t msg_count_;
    double last_time_;

    double latency_sum_ = 0.0;
    double latency_max_ = 0.0;
    uint64_t latency_count_ = 0;

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SensorGatewayNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}