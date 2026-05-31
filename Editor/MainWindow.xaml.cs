using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;

namespace Editor;

public partial class MainWindow : Window
{
    private readonly MainViewModel _viewModel;
    private readonly TcpEditorServer _tcpServer;

    public MainWindow()
    {
        InitializeComponent();

        _viewModel = new MainViewModel();
        DataContext = _viewModel;

        _tcpServer = new TcpEditorServer(_viewModel);

        Loaded += MainWindow_Loaded;
        Closed += MainWindow_Closed;
        SizeChanged += MainWindow_SizeChanged;
    }

    private async void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
        try
        {
            await _tcpServer.StartAsync(7777);

            IntPtr hwnd = new WindowInteropHelper(this).Handle;

            await _tcpServer.SendAsync(new
            {
                type = "create_viewport",
                parentHwnd = hwnd.ToInt64(),
                width = 800,
                height = 600
            });
        }
        catch (Exception ex)
        {
            _viewModel.AddLog("Editor", "MainWindow.xaml.cs", ex.Message);
        }
    }

    private async void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        if (!_tcpServer.IsConnected)
            return;

        await _tcpServer.SendAsync(new
        {
            type = "resize_viewport",
            width = (int)ActualWidth,
            height = (int)ActualHeight
        });
    }

    private void MainWindow_Closed(object? sender, EventArgs e)
    {
        _ = _tcpServer.SendAsync(new
        {
            type = "shutdown"
        });

        _tcpServer.Stop();
    }
}