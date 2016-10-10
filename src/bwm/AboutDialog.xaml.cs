/*
   File: AboutDialog.xaml.cs 
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

using System.IO;
using System.Text;
using System.Windows;

namespace bwm
{
    /// <summary>
    /// Interaction logic for AboutDialog.xaml
    /// </summary>
    public partial class AboutDialog : Window
    {
        public AboutDialog()
        {
            InitializeComponent();

            versionLabel.Content = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString();
            apacheLicense.Selection.Load(new MemoryStream(ASCIIEncoding.Default.GetBytes(global::bwm.Properties.Resources.LICENSE)), DataFormats.Rtf);
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void bwmHyperlink_RequestNavigate(object sender, System.Windows.Navigation.RequestNavigateEventArgs e)
        {
            System.Diagnostics.Process.Start(e.Uri.ToString());
        }
    }
}
