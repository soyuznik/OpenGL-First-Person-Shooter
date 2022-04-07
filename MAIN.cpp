
#include <stdio.h>
#include "./UTILITY.h"
int main() {
	int result = RENDER_GRAPHIC_CONTEXT();
	if (result != 0) {
		std::cout << "Failed to render graphic context \n";
}
    
    if (!glfwInit())
        return 1;

}