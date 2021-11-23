# https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/373/20211122-073732/install.tgz

# GLSLC_CLANG_GAPI="https://storage.googleapis.com/shaderc/badges/build_link_linux_clang_release.html"
# GLSLC_GCC_GAPI="https://storage.googleapis.com/shaderc/badges/build_link_linux_gcc_release.html"

GLSLC_CLANG_GAPI="https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/373/20211122-073732/install.tgz"
# GLSLC_GCC_GAPI="https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_gcc_release/373/20211122-073730/install.tgz"

mkdir tmp

wget -O tmp/shaderc_clang.tgz $GLSLC_CLANG_GAPI
# wget -O tmp/shaderc_gcc.tgz $GLSLC_GCC_GAPI

tar -xvf tmp/shaderc_clang.tgz install/bin/glslc
cp install/bin/glslc /usr/bin

rm -r install
rm -r tmp