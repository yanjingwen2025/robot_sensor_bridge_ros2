#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_gateway/srv/get_sensor_stats.hpp"

using namespace std::chrono_literals;

class ServiceClientNode : public rclcpp::Node {
public:
    ServiceClientNode() : Node("service_client_node") {
        // 创建服务客户端
        client_ = this->create_client<sensor_gateway::srv::GetSensorStats>("get_sensor_stats");

        // 定时器：每5秒调用一次服务
        timer_ = this->create_wall_timer(5s, std::bind(&ServiceClientNode::call_service, this));

        RCLCPP_INFO(this->get_logger(), "Service Client Node started.");
    }

private:
    void call_service() {
        // 检查服务是否可用
        if (!client_->wait_for_service(1s)) {
            RCLCPP_WARN(this->get_logger(), "Service 'get_sensor_stats' not available.");
            return;
        }

        // 创建请求
        auto request = std::make_shared<sensor_gateway::srv::GetSensorStats::Request>();
        // 该服务没有请求参数，所以request为空

        // 异步调用服务
        auto future = client_->async_send_request(
            request,
            std::bind(&ServiceClientNode::service_response_callback, this,
                      std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Service called.");
    }

    void service_response_callback(
        rclcpp::Client<sensor_gateway::srv::GetSensorStats>::SharedFuture future) {
        try {
            auto response = future.get();
            RCLCPP_INFO(this->get_logger(),
                        "Service response: total_messages=%u, last_timestamp=%.6f, status=%s",
                        response->total_messages,
                        response->last_timestamp,
                        response->status.c_str());
        } catch (const std::exception &e) {
            RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", e.what());
        }
    }

    rclcpp::Client<sensor_gateway::srv::GetSensorStats>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ServiceClientNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}