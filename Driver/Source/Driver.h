#ifndef DRIVER
#define DRIVER

namespace Driver {
	struct Request {
		HANDLE ProcessID;
		PVOID Target;
		PVOID Buffer;
		SIZE_T Size;
		SIZE_T ReturnSize;
	};

}

namespace Driver::Codes {

	// IOCTL (I/O Control) Codes
	constexpr ULONG Attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	constexpr ULONG Read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	constexpr ULONG Write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
}

#endif // !DRIVER
