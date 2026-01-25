ARCH_TARGET=${1:-X64}
BUILD_TARGET=${2:-DEBUG}

set -- # Remove positional arguments to prevent issues with edksetup

echo "BUILD OPTIONS:"
echo "  ARCH_TARGET:    ${ARCH_TARGET}"
echo "  BUILD_TARGET:   ${BUILD_TARGET}"

echo
echo

cd ..
source ./edksetup.sh
build -p SnakePkg/SnakePkg.dsc -t GCC -a $ARCH_TARGET -b $BUILD_TARGET -Y COMPILE_INFO -y BuildReport.log  # the parameter "-Y COMPILE_INFO" is for the EDK2Code VSCode extension integration

exit $?
