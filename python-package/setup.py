import os
import platform
import sys
from setuptools import setup, find_packages

this_directory = os.path.abspath(os.path.dirname(__file__))
with open(os.path.join(this_directory, 'README.md')) as f:
    long_description = f.read()

setup(
    name='brainflow',
    version='0.0.1',  # replaced in CI
    description='Python Binding for BrainFlow',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/brainflow-dev/brainflow',
    author='Andrey Parfenov',
    author_email='a1994ndrey@gmail.com',
    packages=find_packages(),
    install_requires=[
        'numpy',
        'nptyping'
    ],
    package_data={
        'brainflow': [
            os.path.join('lib', 'BoardController.dll'),
            os.path.join('lib', 'BoardController32.dll'),
            os.path.join('lib', 'libBoardController.so'),
            os.path.join('lib', 'libBoardController.dylib'),
            os.path.join('lib', 'DataHandler.dll'),
            os.path.join('lib', 'DataHandler32.dll'),
            os.path.join('lib', 'libDataHandler.so'),
            os.path.join('lib', 'libDataHandler.dylib'),
            os.path.join('lib', 'neurosdk-x64.dll'),
            os.path.join('lib', 'neurosdk-x86.dll'),
            os.path.join('lib', 'gForceSDKWrapper32.dll'),
            os.path.join('lib', 'gForceSDKWrapper.dll'),
            os.path.join('lib', 'gforce64.dll'),
            os.path.join('lib', 'gforce32.dll'),
            os.path.join('lib', 'libneurosdk-shared.dylib'),
            os.path.join('lib', 'libunicorn.so'),
            os.path.join('lib', 'libunicorn_raspberry.so'),
            os.path.join('lib', 'Unicorn.dll'),
            os.path.join('lib', 'GanglionLib.dll'),
            os.path.join('lib', 'GanglionLib32.dll'),
            os.path.join('lib', 'libGanglionLib.so'),
            os.path.join('lib', 'libGanglionLib.dylib'),
            os.path.join('lib', 'MLModule.dll'),
            os.path.join('lib', 'brainflow_svm.model'),
            os.path.join('lib', 'libMLModule.so'),
            os.path.join('lib', 'libMLModule.dylib'),
            os.path.join('lib', 'BrainBitLib.dll'),
            os.path.join('lib', 'BrainBitLib32.dll'),
            os.path.join('lib', 'libBrainBitLib.so'),
            os.path.join('lib', 'libBrainBitLib.dylib'),
            os.path.join('lib', 'MuseLib.dll'),
            os.path.join('lib', 'MuseLib32.dll'),
            os.path.join('lib', 'libMuseLib.so'),
            os.path.join('lib', 'libMuseLib.dylib'),
            os.path.join('lib', 'eego-SDK32.dll'),
            os.path.join('lib', 'eego-SDK.dll'),
            os.path.join('lib', 'libeego-SDK.so'),
            os.path.join('lib', 'libeego-SDK32.so')
        ]
    },
    zip_safe=True,
    python_requires='>=3.5.6'
)
