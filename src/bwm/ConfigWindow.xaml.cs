/*
   File: ConfigWindow.xaml.cs 
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

using Microsoft.Win32;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;

namespace bwm
{
    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum MouseButton
    {
        [Description("Not Enabled")]
        None = 0,
        [Description("Left Mouse Button")]
        LeftButton = 1,
        [Description("Right Mouse Button")]
        RightButton = 2
    };

    /// <summary>
    /// Interaction logic for ConfigWindow.xaml
    /// </summary>
    public partial class ConfigWindow : Window
    {
        HookHandler mHook = new HookHandler();

        public ConfigWindow()
        {
            InitializeComponent();
            this.Hide();

            mHook.Hook();
            UpdateAllSettings();
            setupNotifyIcon();

            Properties.Settings.Default.PropertyChanged += SettingsChanged;
        }

        private void CheckEquality( MouseButton val1, MouseButton val2, String err, ComboBox ctrl)
        {
            if (val1.Equals(val2))
            {
                ValidationError validationError =
                    new ValidationError(new ExceptionValidationRule(), ctrl.GetBindingExpression(ComboBox.SelectedItemProperty));

                validationError.ErrorContent = err;

                Validation.MarkInvalid(ctrl.GetBindingExpression(ComboBox.SelectedItemProperty),
                                       validationError);
            }
            else
            {
                Validation.ClearInvalid(ctrl.GetBindingExpression(ComboBox.SelectedItemProperty));
            }

        }

        private void SettingsChanged(object sender, PropertyChangedEventArgs e)
        {
            CheckEquality(Properties.Settings.Default.ResizeWindowButton, Properties.Settings.Default.MoveWindowButton,
                           "Resize & Move must use different buttons", moveWindowComboBox);
            CheckEquality(Properties.Settings.Default.MinimiseWindowButton, Properties.Settings.Default.MaximiseWindowButton,
                            "Maximise & Minimise must use different buttons", maximiseWindowComboBox);
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_closeReason == CloseReason.User)
            {
                e.Cancel = true;
                this.Hide();
            }
        }

        System.Windows.Forms.NotifyIcon notifyIcon;

        void setupNotifyIcon()
        {
            // TODO: Add ability to choose an arrangement to apply from the menu

            // TODO: Add import/export function from menu

            // TODO: Add keyboard shortcuts to menu.

            System.Windows.Forms.MenuItem aboutMenuItem = new System.Windows.Forms.MenuItem("About", new EventHandler(ShowAbout));
            System.Windows.Forms.MenuItem optionsMenuItem = new System.Windows.Forms.MenuItem("Options", new EventHandler(ShowOptions));
            System.Windows.Forms.MenuItem exitMenuItem = new System.Windows.Forms.MenuItem("Exit", new EventHandler(Exit));

            notifyIcon = new System.Windows.Forms.NotifyIcon();
            notifyIcon.Icon = Properties.Resources.icon1;
            notifyIcon.ContextMenu = new System.Windows.Forms.ContextMenu(new System.Windows.Forms.MenuItem[] { aboutMenuItem, optionsMenuItem, exitMenuItem });
            notifyIcon.Visible = true;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            HwndSource source = (HwndSource)PresentationSource.FromDependencyObject(this);
            source.AddHook(WindowProc);
        }

        enum CloseReason
        {
            EndTask,
            Logoff,
            User
        };
        CloseReason _closeReason;

        private IntPtr WindowProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch (msg)
            {
                case 0x11:
                case 0x16:
                    _closeReason = CloseReason.Logoff;
                    break;

                case 0x112:
                    if ((LOWORD((int)wParam) & 0xfff0) == 0xf060)
                        _closeReason = CloseReason.User;
                    break;
            }
            return IntPtr.Zero;
        }

        void ShowOptions(object sender, EventArgs e)
        {
            this.Show();
        }

        void ShowAbout(object sender, EventArgs e)
        {
            AboutDialog ad = new AboutDialog();
            ad.ShowDialog();
        }

        void Exit(object sender, EventArgs e)
        {
            _closeReason = CloseReason.EndTask;
            notifyIcon.Visible = false;
            this.Close();
        }

        private static int LOWORD(int n)
        {
            return (n & 0xffff);
        }

        private void UpdateRunAtStartup()
        {
            RegistryKey rkApp = Registry.CurrentUser.OpenSubKey(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Run", true);

            if (Properties.Settings.Default.RunAtStartup)
            {
                rkApp.SetValue("BWM", "\"" + System.Reflection.Assembly.GetExecutingAssembly().Location + "\"");
            }
            else
            {
                rkApp.DeleteValue("BWM", false);
            }
        }

        private void UpdateMouseBindings()
        {
            mHook.SetWindowResizeMouseButton((int)Properties.Settings.Default.ResizeWindowButton);
            mHook.SetWindowMoveMouseButton((int)Properties.Settings.Default.MoveWindowButton);
            mHook.SetWindowMinimiseMouseButton((int)Properties.Settings.Default.MinimiseWindowButton);
            mHook.SetWindowMaximiseMouseButton((int)Properties.Settings.Default.MaximiseWindowButton);
        }

        private void UpdateAllSettings()
        {
            UpdateKeyboardModifiers();
            UpdateMouseBindings();
            UpdateScreenSnap();
            UpdateWindowSnap();
            UpdateRunAtStartup();
        }

        private void UpdateScreenSnap()
        {
            int snapVal = Properties.Settings.Default.ScreenSnapMargin;
            if(! Properties.Settings.Default.ScreenSnapEnabled)
            {
                snapVal = 0;
            }
            mHook.SetScreenSnapMargin(snapVal);
        }

        private void UpdateWindowSnap()
        {
            int snapVal = Properties.Settings.Default.WindowSnapMargin;
            if (!Properties.Settings.Default.WindowSnapEnabled)
            {
                snapVal = 0;
            }
            mHook.SetWindowSnapMargin(snapVal);
        }

        private void UpdateKeyboardModifiers()
        {
            int count = 0;
            int[] mods = new int[10];

            if (Properties.Settings.Default.ModifierShift)
            {
                mods[count] = 0x10;
                count++;
            }
            if (Properties.Settings.Default.ModifierAlt)
            {
                mods[count] = 0x12;
                count++;
            }
            if (Properties.Settings.Default.ModifierCtrl)
            {
                mods[count] = 0x11;
                count++;
            }
            if (Properties.Settings.Default.ModifierWin)
            {
                mods[count] = 0x5b;
                count++;
            }

            mHook.SetKeyboardModifiers(count, mods);

        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            bwm.Properties.Settings.Default.Save();

            UpdateAllSettings();
        }
    }
}
