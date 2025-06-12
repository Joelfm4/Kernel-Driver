#ifndef LOG
#define LOG

namespace Log {
	const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

	inline void Info(std::string Ctx) {
		SetConsoleTextAttribute(Console, 0x9);
		std::cout << "[Info] ";

		SetConsoleTextAttribute(Console, 0x7);
		std::cout << Ctx << '\n';
	}

	inline void Fine(std::string Ctx) {
		SetConsoleTextAttribute(Console, 0xA);
		std::cout << "[Fine] ";

		SetConsoleTextAttribute(Console, 0x7);
		std::cout << Ctx << '\n';
	}

	inline void Warn(std::string Ctx) {
		SetConsoleTextAttribute(Console, 0x6);
		std::cout << "[Warn] ";

		SetConsoleTextAttribute(Console, 0x7);
		std::cout << Ctx << '\n';
	}

	inline void Error(std::string Ctx, bool Fatal = true) {
		SetConsoleTextAttribute(Console, 0x4);
		std::cout << "[Error] ";

		SetConsoleTextAttribute(Console, 0x7);
		std::cout << Ctx << '\n';

		if (Fatal) {
			system("pause");
			exit(0);
		}

	}
};


#endif // !LOG
