#!/usr/bin/env python3

from setuptools import setup
from catkin_pkg.python_setup import generate_distutils_setup

setup_args = generate_distutils_setup(
    packages=["screen_grab"],
    package_dir={"": "src"},
    scripts=["src/screen_grab/main.py"],  # Allows `rosrun screen_grab main.py`
)

setup(**setup_args)
