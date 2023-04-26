// 生成.h和.cpp，供我们自用
protoc --cpp_out=..\..\..\..\bililive\public\protobuf -I=..\..\..\..\bililive\public\protobuf streaming_details.proto

//当前proto文件目录下  protoc --cpp_out=. broadcast.proto

// 生成一堆.js文件供WEB端使用
protoc --js_out=..\..\..\..\bililive\public\protobuf -I=..\..\..\..\bililive\public\protobuf streaming_details.proto