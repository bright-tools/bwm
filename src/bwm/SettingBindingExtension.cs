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

            this.ValidatesOnDataErrors = true;
            this.NotifyOnValidationError = true;
        }

        private void Initialize()
        {
            this.Source = bwm.Properties.Settings.Default;
            this.Mode = BindingMode.TwoWay;
        }
    }
}
