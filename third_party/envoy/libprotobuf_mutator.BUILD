load("@rules_cc//cc:defs.bzl", "cc_library")

licenses(["notice"])  # Apache 2

cc_library(
    name = "libprotobuf_mutator",
    srcs = glob(
        [
            "src/**/*.cc",
            "src/**/*.h",
            "port/protobuf.h",
        ],
        exclude = ["**/*_test.cc"],
    ),
    hdrs = ["src/libfuzzer/libfuzzer_macro.h"],
    include_prefix = "libprotobuf_mutator",
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:protobuf"],
)