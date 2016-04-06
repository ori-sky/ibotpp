#include <iostream>

MODULE(
	"test", // name
	{       // handlers
		{"test", [] { std::cout << "hello world" << std::endl; }
	}
})
