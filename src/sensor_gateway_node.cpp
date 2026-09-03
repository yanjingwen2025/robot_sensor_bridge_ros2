#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_gateway/srv/get_sensor_stats.hpp"

using namespace std::chrono_literals;

class SensorGatewayNode : public rclcpp::Node {
public:
    SensorGatewayNode() : Node("sensor_gateway_node"), msg_count_(0), last_time_(0.0) {
        // 1. 订阅者：接收 IMU 数据
        subscription_ = this->create_subscription<sensor_msgs::msg::Imu>(
            "/imu/data", 10,
            std::bind(&SensorGatewayNode::imu_callback, this, std::placeholders::_1));

        // 2. 发布者：发布统计摘要
        publisher_ = this->create_publisher<std_msgs::msg::String>("/sensor_summary", 10);

        // 3. 服务服务器：提供状态查询
        service_ = this->create_service<sensor_gateway::srv::GetSensorStats>(
            "get_sensor_stats",
            std::bind(&SensorGatewayNode::service_callback, this,
                      std::placeholders::_1, std::placeholders::_2));

        // 4. 定时器：每 2 秒发布一次摘要
        timer_ = this->create_wall_timer(2s, std::bind(&SensorGatewayNode::publish_summary, this));

        RCLCPP_INFO(this->get_logger(), "Sensor Gateway Node started.");
    }

private:
    void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg) {
        msg_count_++;
        last_time_ = msg->header.stamp.sec + msg->header.stamp.nanosec * 1e-9;
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

    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::Service<sensor_gateway::srv::GetSensorStats>::SharedPtr service_;
    rclcpp::TimerBase::SharedPtr timer_;

    uint32_t msg_count_;
    double last_time_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SensorGatewayNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}