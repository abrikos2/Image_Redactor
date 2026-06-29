
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y \
    cmake \
    g++ \
    libopencv-dev \
    qt6-base-dev \
    libgl1-mesa-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY main.cpp .
COPY include/ include/
COPY src/ src/
COPY tests/ tests/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --config Release -j$(nproc)

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y \
    qt6-qpa-plugins \
    libopencv-dev \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libgl1-mesa-glx \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/Image_Redactor .

ENTRYPOINT ["./Image_Redactor"]