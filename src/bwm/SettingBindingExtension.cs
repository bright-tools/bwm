/*
   File: SettinBindingExtension.cs 
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
