from setuptools import find_packages, setup

package_name = 'my_first_package'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        (
            'share/ament_index/resource_index/packages',
            ['resource/' + package_name],
        ),
        (
            'share/' + package_name,
            ['package.xml'],
        ),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='nick',
    maintainer_email='nick@example.com',
    description='ROS2 robotic arm test package',
    license='Apache-2.0',
    tests_require=['pytest'],

   entry_points={
    'console_scripts': [
        'joint_command = my_first_package.testing:main',
        'simulated_joint = my_first_package.simulated_joint:main',
    ],
},
)