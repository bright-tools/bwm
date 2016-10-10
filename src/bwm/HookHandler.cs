/*
   File: HookHandler.cs 
   Project: BWM - https://github.com/bright-tools/bwm

   Copyright 2016 Bright Silence Limited

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software 
  distributed under the License is distributed on an "AS IS" BASIS, 
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

  See the License for the specific language governing permissions and 
  limitations under the License.
*/

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

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetScreenSnap(int val);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetWindowSnap(int val);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetResizeMButton(int val);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetMoveMButton(int val);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetMaximiseMButton(int val);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetMinimiseMButton(int val);

        private IntPtr mDll = IntPtr.Zero;
        private IntPtr mAddressOfSetMouseHook;
        private IntPtr mAddressOfRemoveMouseHook;
        private IntPtr mAddressOfGetInstanceCount;
        private IntPtr mAddressOfSetModifiers;
        private IntPtr mAddressOfSetScreenSnap;
        private IntPtr mAddressOfSetWindowSnap;
        private IntPtr mAddressOfSetMoveMButton;
        private IntPtr mAddressOfSetResizeMButton;
        private IntPtr mAddressOfSetMaximiseMButton;
        private IntPtr mAddressOfSetMinimiseMButton;

        SetMouseHook m_setMouseHook = null;
        RemoveMouseHook m_removeMouseHook = null;
        GetInstanceCount m_getInstanceCount = null;
        SetModifiers m_setModifiers = null;
        SetScreenSnap m_setScreenSnap = null;
        SetWindowSnap m_setWindowSnap = null;
        SetMaximiseMButton m_setMaximiseMButton = null;
        SetMinimiseMButton m_setMinimiseMButton = null;
        SetResizeMButton m_setResizeMButton = null;
        SetMoveMButton m_setMoveMButton = null;

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

        public void SetScreenSnapMargin(int px)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setScreenSnap(px);
            }
        }

        public void SetWindowSnapMargin(int px)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setWindowSnap(px);
            }
        }

        public void SetWindowMoveMouseButton(int px)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setMoveMButton(px);
            }
        }

        public void SetWindowResizeMouseButton(int px)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setResizeMButton(px);
            }
        }

        public void SetWindowMaximiseMouseButton(int px)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setMaximiseMButton(px);
            }
        }

        public void SetWindowMinimiseMouseButton(int px)
        {
            if (mDll != IntPtr.Zero)
            {
                m_setMinimiseMButton(px);
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

        protected void GetProcedureAddresses()
        {
            mAddressOfSetMouseHook = NativeMethods.GetProcAddress(mDll, "SetMouseHook");
            mAddressOfRemoveMouseHook = NativeMethods.GetProcAddress(mDll, "RemoveMouseHook");
            mAddressOfGetInstanceCount = NativeMethods.GetProcAddress(mDll, "GetInstanceCount");
            mAddressOfSetModifiers = NativeMethods.GetProcAddress(mDll, "SetModifiers");
            mAddressOfSetScreenSnap = NativeMethods.GetProcAddress(mDll, "SetScreenSnap");
            mAddressOfSetWindowSnap = NativeMethods.GetProcAddress(mDll, "SetWindowSnap");
            mAddressOfSetMoveMButton = NativeMethods.GetProcAddress(mDll, "SetMoveMButton");
            mAddressOfSetResizeMButton = NativeMethods.GetProcAddress(mDll, "SetResizeMButton");
            mAddressOfSetMaximiseMButton = NativeMethods.GetProcAddress(mDll, "SetMaximiseMButton");
            mAddressOfSetMinimiseMButton = NativeMethods.GetProcAddress(mDll, "SetMinimiseMButton");
        }

        protected bool CheckProcedureAddresses()
        {
            return (mAddressOfSetWindowSnap == IntPtr.Zero) ||
                   (mAddressOfSetScreenSnap == IntPtr.Zero) ||
                   (mAddressOfSetMouseHook == IntPtr.Zero) ||
                   (mAddressOfRemoveMouseHook == IntPtr.Zero) ||
                   (mAddressOfGetInstanceCount == IntPtr.Zero) ||
                   (mAddressOfSetModifiers == IntPtr.Zero) ||
                   (mAddressOfSetMoveMButton == IntPtr.Zero) ||
                   (mAddressOfSetResizeMButton == IntPtr.Zero) ||
                   (mAddressOfSetMaximiseMButton == IntPtr.Zero) ||
                   (mAddressOfSetMinimiseMButton == IntPtr.Zero);
        }

        protected void GetHooks()
        {
            m_setMouseHook = (SetMouseHook)Marshal.GetDelegateForFunctionPointer(mAddressOfSetMouseHook, typeof(SetMouseHook));
            m_removeMouseHook = (RemoveMouseHook)Marshal.GetDelegateForFunctionPointer(mAddressOfRemoveMouseHook, typeof(RemoveMouseHook));
            m_getInstanceCount = (GetInstanceCount)Marshal.GetDelegateForFunctionPointer(mAddressOfGetInstanceCount, typeof(GetInstanceCount));
            m_setModifiers = (SetModifiers)Marshal.GetDelegateForFunctionPointer(mAddressOfSetModifiers, typeof(SetModifiers));
            m_setScreenSnap = (SetScreenSnap)Marshal.GetDelegateForFunctionPointer(mAddressOfSetScreenSnap, typeof(SetScreenSnap));
            m_setWindowSnap = (SetWindowSnap)Marshal.GetDelegateForFunctionPointer(mAddressOfSetWindowSnap, typeof(SetWindowSnap));
            m_setMoveMButton = (SetMoveMButton)Marshal.GetDelegateForFunctionPointer(mAddressOfSetMoveMButton, typeof(SetMoveMButton));
            m_setResizeMButton = (SetResizeMButton)Marshal.GetDelegateForFunctionPointer(mAddressOfSetResizeMButton, typeof(SetResizeMButton));
            m_setMaximiseMButton = (SetMaximiseMButton)Marshal.GetDelegateForFunctionPointer(mAddressOfSetMaximiseMButton, typeof(SetMaximiseMButton));
            m_setMinimiseMButton = (SetMinimiseMButton)Marshal.GetDelegateForFunctionPointer(mAddressOfSetMinimiseMButton, typeof(SetMinimiseMButton));
        }

        bool LoadDLL()
        {
            bool retVal = false;

            if (mDll == IntPtr.Zero)
            {
                mDll = NativeMethods.LoadLibrary(GetExecutingDirectory()+@"\wmmouse.dll");

                if (mDll != IntPtr.Zero)
                {
                    GetProcedureAddresses();

                    if ( CheckProcedureAddresses() )
                    {
                        UnloadDLL();
                    }
                    else
                    {
                        GetHooks();
                        retVal = true;
                    }
                }
            }
            return retVal;
        }

    }
}
