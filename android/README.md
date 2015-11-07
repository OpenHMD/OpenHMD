# Generate a build.xml
android update project --path . --subprojects --target android-19

# Build ndk stuff
ndk-build

# Build java stuff
ant debug

# Install onto device
ant installd
