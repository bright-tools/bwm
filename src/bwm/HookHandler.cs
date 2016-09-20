using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace bwm
{
    class HookHandler
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetMouseHook();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void RemoveMouseHook();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int GetInstanceCount();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int SetModifiers(int count, int[] mods);


        private IntPtr mDll = IntPtr.Zero;
        private IntPtr mAddressOfSetMouseHook;
        private IntPtr mAddressOfRemoveMouseHook;
        private IntPtr mAddressOfGetInstanceCount;
        private IntPtr mAddressOfSetModifiers;

        SetMouseHook m_setMouseHook = null;
        RemoveMouseHook m_removeMouseHook = null;
        GetInstanceCount m_getInstanceCount = null;
        SetModifiers m_setModifiers = null;

        public void Hook()
        {
            if (LoadDLL())
            {
                int iCount = m_getInstanceCount();
                if (m_getInstanceCount() == 1)
                {
                    m_setMouseHook();
                }
            }
        }

        public void SetKeyboardModifiers(int count, int[] mods)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setModifiers(count, mods);
            }
        }

        public void UnHook()
        {
            UnloadDLL();
        }

        void UnloadDLL()
        {
            if (mDll != IntPtr.Zero)
            {
                NativeMethods.FreeLibrary(mDll);
                mDll = IntPtr.Zero;
            }
        }

        protected static DirectoryInfo GetExecutingDirectory()
        {
            Uri location = new Uri(Assembly.GetEntryAssembly().GetName().CodeBase);
            return new FileInfo(Uri.UnescapeDataString(location.AbsolutePath)).Directory;
        }

        bool LoadDLL()
        {
            bool retVal = false;

            if (mDll == IntPtr.Zero)
            {
                mDll = NativeMethods.LoadLibrary(GetExecutingDirectory()+@"\wmmouse.dll");

                if (mDll != IntPtr.Zero)
                {
                    mAddressOfSetMouseHook = NativeMethods.GetProcAddress(mDll, "SetMouseHook");
                    mAddressOfRemoveMouseHook = NativeMethods.GetProcAddress(mDll, "RemoveMouseHook");
                    mAddressOfGetInstanceCount = NativeMethods.GetProcAddress(mDll, "GetInstanceCount");
                    mAddressOfSetModifiers = NativeMethods.GetProcAddress(mDll, "SetModifiers");

                    if ((mAddressOfSetMouseHook == IntPtr.Zero) || (mAddressOfRemoveMouseHook == IntPtr.Zero) || (mAddressOfGetInstanceCount == IntPtr.Zero) || (mAddressOfSetModifiers == IntPtr.Zero))
                    {
                        UnloadDLL();
                    }
                    else
                    {
                        m_setMouseHook = (SetMouseHook)Marshal.GetDelegateForFunctionPointer(mAddressOfSetMouseHook, typeof(SetMouseHook));
                        m_removeMouseHook = (RemoveMouseHook)Marshal.GetDelegateForFunctionPointer(mAddressOfRemoveMouseHook, typeof(RemoveMouseHook));
                        m_getInstanceCount = (GetInstanceCount)Marshal.GetDelegateForFunctionPointer(mAddressOfGetInstanceCount, typeof(GetInstanceCount));
                        m_setModifiers = (SetModifiers)Marshal.GetDelegateForFunctionPointer(mAddressOfSetModifiers, typeof(SetModifiers));
                        retVal = true;
                    }
                }
            }
            return retVal;
        }

    }
}
