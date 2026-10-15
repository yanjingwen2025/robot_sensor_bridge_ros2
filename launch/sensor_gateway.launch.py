from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node



def generate_launch_description():

    imu_use_reliable_arg = DeclareLaunchArgument(
        'imu_use_reliable',
        default_value='true',
        description='IMU subscription QoS: true=RELIABLE, false=BEST_EFFORT'
    )

    return LaunchDescription([
        imu_use_reliable_arg,
        Node(
            package='sensor_gateway',
            executable='sensor_gateway_node',
            name='sensor_gateway_node',
            output='screen',
            
            parameters=[{
                'imu_topic': '/imu/data',
                'summary_topic': '/sensor_summary',
                'publish_period': 2.0,
                 'imu_use_reliable': LaunchConfiguration('imu_use_reliable'),
            }]
        ),
       Node(
            package='sensor_gateway',
            executable='data_processor_node',
            name='data_processor_node',
            output='screen',
            
            parameters=[{
                'input_topic': '/sensor_summary',
                'output_topic': '/processed_result',
            }],
        ),
        Node(
            package='sensor_gateway',
            executable='service_client_node',
            name='service_client_node',
            output='screen',
            
            parameters=[{
                'service_name': 'get_sensor_stats',
                'call_period': 5.0,
            }],
        ),
    ])