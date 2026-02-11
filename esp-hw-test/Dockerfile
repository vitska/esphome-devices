# ESP8266 PlatformIO Build Environment
FROM python:3.11-slim

# Install required packages
RUN apt-get update && apt-get install -y \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install PlatformIO
RUN pip install --no-cache-dir platformio

# Pre-install ESP8266 platform to speed up builds
RUN pio pkg install -g -p espressif8266

# Set working directory
WORKDIR /project

# Default command: build the project
CMD ["pio", "run"]
