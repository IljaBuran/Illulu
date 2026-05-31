using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Text;

namespace Editor
{
    public class MainViewModel
    {
        public ObservableCollection<LogEntry> Logs { get; } = new();

        public MainViewModel()
        {
            AddTestLogs();
        }

        private void AddTestLogs()
        {
            Logs.Add(new LogEntry
            {
                Time = DateTime.Now.ToString("HH:mm:ss"),
                Origin = "Engine",
                File = "Engine.cpp",
                Description = "Engine initialized successfully."
            });

            Logs.Add(new LogEntry
            {
                Time = DateTime.Now.ToString("HH:mm:ss"),
                Origin = "Renderer",
                File = "Renderer.cpp",
                Description = "Viewport framebuffer resized to 1920x1080."
            });

            Logs.Add(new LogEntry
            {
                Time = DateTime.Now.ToString("HH:mm:ss"),
                Origin = "AssetManager",
                File = "TextureLoader.cpp",
                Description = "Loaded texture: Assets/Textures/wood_albedo.png"
            });

            Logs.Add(new LogEntry
            {
                Time = DateTime.Now.ToString("HH:mm:ss"),
                Origin = "Logger",
                File = "EditorBridge.cpp",
                Description = "This is a very long test message to check how the description column wraps inside the dark DataGrid logger."
            });
        }

        public void AddLog(string origin, string file, string description)
        {
            Logs.Add(new LogEntry
            {
                Time = DateTime.Now.ToString("HH:mm:ss"),
                Origin = origin,
                File = file,
                Description = description
            });
        }

    }
}
