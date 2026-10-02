import rclpy
from rclpy.node import Node

from std_msgs.msg import Float64
from sensor_msgs.msg import JointState

import math


class SimulatedJoint(Node):

    def __init__(self):
        super().__init__('simulated_joint_node')

        self.subscription = self.create_subscription(
            Float64,
            '/joint1/command',
            self.command_callback,
            10
        )

        self.publisher_ = self.create_publisher(
            JointState,
            '/joint_states',
            10
        )

        self.current_position = 0.0
        self.target_position = 0.0

        self.k = 2.0
        self.dt = 0.02

        self.timer = self.create_timer(
            self.dt,
            self.update_joint
        )

        self.get_logger().info(
            'Simulated Joint 1 started'
        )

    def command_callback(self, msg):
        self.target_position = msg.data

        self.get_logger().info(
            f'Received target: {math.degrees(self.target_position):.1f} deg'
        )

    def update_joint(self):
        error = self.target_position - self.current_position

        velocity = self.k * error

        self.current_position += velocity * self.dt

        msg = JointState()

        msg.header.stamp = self.get_clock().now().to_msg()
        msg.name = ['joint1']
        msg.position = [self.current_position]
        msg.velocity = [velocity]

        self.publisher_.publish(msg)


def main(args=None):
    rclpy.init(args=args)

    node = SimulatedJoint()

    rclpy.spin(node)

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()