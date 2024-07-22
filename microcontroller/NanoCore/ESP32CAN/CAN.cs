using System;
using System.Runtime.CompilerServices;

namespace ESP32CAN {
	public class CAN {
		public static void Init() {
			InitNative();
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		static extern void InitNative();
	}
}
