#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class DataProcessorNode : public rclcpp::Node {
public:
    DataProcessorNode() : Node("data_processor_node") {
        // 订阅统计摘要话题
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/sensor_summary", 10,
            std::bind(&DataProcessorNode::summary_callback, this, std::placeholders::_1));

        // 发布处理后的结果
        publisher_ = this->create_publisher<std_msgs::msg::String>("/processed_result", 10);

        RCLCPP_INFO(this->get_logger(), "Data Processor Node started.");
    }

private:
    void summary_callback(const std_msgs::msg::String::SharedPtr msg) {
        // 在原始消息前加前缀
        auto processed_msg = std_msgs::msg::String();
        processed_msg.data = "[PROCESSED] " + msg->data;
        
        publisher_->publish(processed_msg);
        
        RCLCPP_INFO(this->get_logger(), "Processed: %s", processed_msg.data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DataProcessorNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}