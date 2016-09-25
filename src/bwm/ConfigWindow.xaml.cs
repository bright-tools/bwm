using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace bwm
{
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

            System.Windows.Forms.MenuItem optionsMenuItem = new System.Windows.Forms.MenuItem("Options", new EventHandler(ShowOptions));
            System.Windows.Forms.MenuItem exitMenuItem = new System.Windows.Forms.MenuItem("Exit", new EventHandler(Exit));

            notifyIcon = new System.Windows.Forms.NotifyIcon();
            notifyIcon.Icon = Properties.Resources.icon1;
            notifyIcon.ContextMenu = new System.Windows.Forms.ContextMenu(new System.Windows.Forms.MenuItem[] { optionsMenuItem, exitMenuItem });
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

        private void UpdateAllSettings()
        {
            UpdateKeyboardModifiers();
            UpdateScreenSnap();
            UpdateWindowSnap();
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
