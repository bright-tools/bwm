/*
   File: App.xaml.cs 
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
using System.Reflection;
using System.Threading;
using System.Windows;

namespace bwm
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : System.Windows.Application
    {
        static Mutex mutex = new Mutex(true, "{" + Assembly.GetExecutingAssembly().GetType().GUID.ToString() + "}");

        public App() : base()
        {
            ShutdownMode = ShutdownMode.OnMainWindowClose;
        }

        /// <summary>
        /// Application Entry Point.
        /// </summary>
        [System.STAThreadAttribute()]
        public static void Main()
        {
            if (mutex.WaitOne(TimeSpan.Zero, true))
            {
                bwm.App app = new bwm.App();
                app.InitializeComponent();
                app.Run();
            }
            else
            {
                MessageBox.Show("BWM is already running.");
            }
        }

    }

}
