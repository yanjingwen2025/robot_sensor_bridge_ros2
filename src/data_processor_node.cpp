#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class DataProcessorNode : public rclcpp::Node {
public:
    DataProcessorNode() : Node("data_processor_node") {
    // 声明参数
    this->declare_parameter<std::string>("input_topic", "/sensor_summary");
    this->declare_parameter<std::string>("output_topic", "/processed_result");

    // 读取参数
    std::string input_topic = this->get_parameter("input_topic").as_string();
    std::string output_topic = this->get_parameter("output_topic").as_string();

    // 用参数创建
    subscription_ = this->create_subscription<std_msgs::msg::String>(
        input_topic, 10,
        std::bind(&DataProcessorNode::summary_callback, this, std::placeholders::_1));

    publisher_ = this->create_publisher<std_msgs::msg::String>(output_topic, 10);

    RCLCPP_INFO(this->get_logger(),
                "Data Processor Node started. input=%s, output=%s",
                input_topic.c_str(), output_topic.c_str());
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