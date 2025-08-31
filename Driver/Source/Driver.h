#pragma once

namespace Driver {

	struct Request {
		HANDLE ProcessID;
		PVOID Target;
		PVOID Buffer;
		SIZE_T Size;
		SIZE_T ReturnSize;
	};

	namespace Codes {

		inline constexpr ULONG Attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		inline constexpr ULONG Read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		inline constexpr ULONG Write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	};
};
