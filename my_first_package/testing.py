import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64
import math


class JointCommandNode(Node):

    def __init__(self):
        super().__init__('joint_command_node')

        self.publisher_ = self.create_publisher(
            Float64,
            '/joint1/command',
            10
        )

        self.targets_deg = [
            0.0,
            90.0,
            -45.0,
            45.0,
            0.0
        ]

        self.index = 0

        self.timer = self.create_timer(
            4.0,
            self.publish_command
        )

        self.get_logger().info(
            'Joint trajectory command node started'
        )

    def publish_command(self):

        target_deg = self.targets_deg[self.index]

        target_rad = math.radians(target_deg)

        msg = Float64()
        msg.data = target_rad

        self.publisher_.publish(msg)

        self.get_logger().info(
            f'Commanding Joint 1 to {target_deg:.1f} degrees'
        )

        self.index += 1

        if self.index >= len(self.targets_deg):
            self.index = 0


def main(args=None):

    rclpy.init(args=args)

    node = JointCommandNode()

    rclpy.spin(node)

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()