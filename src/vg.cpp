#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>

class TriangleApp {
public:
  void run() {
    init();
    loop();
    cleanup();
  }

private:
  void init() {}
  void loop() {}
  void cleanup() {}
};

int main() {
  TriangleApp app;
  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
