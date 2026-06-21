# ============================================================
# Multi-stage Dockerfile for Image_Redactor (Linux x64)
# Stage 1: Builder
# Stage 2: Runtime
# ============================================================

# ---- Stage 1: Builder ----
FROM ubuntu:22.04 AS builder

# ИСПРАВЛЕНО: Отключаем интерактивные запросы (выбор таймзоны)
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    libopencv-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy only CMakeLists.txt and source files first (for layer caching)
COPY CMakeLists.txt .
COPY main.cpp .
COPY include/ include/
COPY src/ src/

# Create build directory and configure
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --config Release -j$(nproc)

# ---- Stage 2: Runtime ----
FROM ubuntu:22.04

# ИСПРАВЛЕНО: Отключаем интерактивные запросы и здесь тоже
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libopencv-dev \
    libgl1-mesa-glx \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy only the built binary from builder stage
COPY --from=builder /app/build/Image_Redactor .

# Set entrypoint
ENTRYPOINT ["./Image_Redactor"]