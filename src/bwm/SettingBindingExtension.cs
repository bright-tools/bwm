using System.Windows.Data;

namespace bwm
{
    public class SettingBindingExtension : Binding
    {
        public SettingBindingExtension()
        {
            Initialize();
        }

        public SettingBindingExtension(string path)
            : base(path)
        {
            Initialize();
        }

        private void Initialize()
        {
            this.Source = bwm.Properties.Settings.Default;
            this.Mode = BindingMode.TwoWay;
        }
    }
}
