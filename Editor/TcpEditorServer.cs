using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace Editor;

public sealed class TcpEditorServer
{
    private readonly MainViewModel _viewModel;
    private TcpListener? _listener;
    private TcpClient? _client;
    private NetworkStream? _stream;
    private CancellationTokenSource? _cts;

    public bool IsConnected => _client?.Connected == true;

    public TcpEditorServer(MainViewModel viewModel)
    {
        _viewModel = viewModel;
    }

    public async Task StartAsync(int port = 7777)
    {
        _cts = new CancellationTokenSource();

        _listener = new TcpListener(IPAddress.Loopback, port);
        _listener.Start();

        _viewModel.AddLog("Editor", "TcpEditorServer.cs", $"TCP server started on 127.0.0.1:{port}");

        _client = await _listener.AcceptTcpClientAsync(_cts.Token);
        _stream = _client.GetStream();

        _viewModel.AddLog("Editor", "TcpEditorServer.cs", "Engine connected.");

        _ = Task.Run(() => ReceiveLoopAsync(_cts.Token));
    }

    public async Task SendAsync(object message)
    {
        if (_stream == null)
            return;

        string json = JsonSerializer.Serialize(message);
        byte[] payload = Encoding.UTF8.GetBytes(json);
        byte[] lengthBytes = BitConverter.GetBytes(payload.Length);

        if (!BitConverter.IsLittleEndian)
            Array.Reverse(lengthBytes);

        await _stream.WriteAsync(lengthBytes);
        await _stream.WriteAsync(payload);
        await _stream.FlushAsync();
    }

    private async Task ReceiveLoopAsync(CancellationToken token)
    {
        try
        {
            while (!token.IsCancellationRequested && _stream != null)
            {
                byte[] lengthBytes = await ReadExactAsync(_stream, 4, token);

                int length = BitConverter.ToInt32(lengthBytes, 0);

                if (length <= 0 || length > 1024 * 1024)
                    throw new InvalidOperationException("Invalid TCP message size.");

                byte[] payload = await ReadExactAsync(_stream, length, token);
                string json = Encoding.UTF8.GetString(payload);

                HandleMessage(json);
            }
        }
        catch (Exception ex)
        {
            App.Current.Dispatcher.Invoke(() =>
            {
                _viewModel.AddLog("Editor", "TcpEditorServer.cs", $"TCP disconnected: {ex.Message}");
            });
        }
    }

    private void HandleMessage(string json)
    {
        using JsonDocument doc = JsonDocument.Parse(json);

        string type = doc.RootElement.GetProperty("type").GetString() ?? "";

        App.Current.Dispatcher.Invoke(() =>
        {
            if (type == "log")
            {
                string origin = doc.RootElement.GetProperty("origin").GetString() ?? "Engine";
                string file = doc.RootElement.GetProperty("file").GetString() ?? "";
                string description = doc.RootElement.GetProperty("description").GetString() ?? "";

                _viewModel.AddLog(origin, file, description);
            }
            else
            {
                _viewModel.AddLog("Engine", "TCP", json);
            }
        });
    }

    private static async Task<byte[]> ReadExactAsync(NetworkStream stream, int size, CancellationToken token)
    {
        byte[] buffer = new byte[size];
        int offset = 0;

        while (offset < size)
        {
            int read = await stream.ReadAsync(buffer.AsMemory(offset, size - offset), token);

            if (read == 0)
                throw new Exception("Remote side closed connection.");

            offset += read;
        }

        return buffer;
    }

    public void Stop()
    {
        _cts?.Cancel();
        _stream?.Close();
        _client?.Close();
        _listener?.Stop();
    }
}