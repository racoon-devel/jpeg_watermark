# jpeg_watermark
Серверное приложение для печати текста на изображениях

Требуется разработать серверное приложение со следующей функциональностью:

* Обработка поступающих сетевых запросов;

* Каждый запрос содержит изображение в формате JPEG и строку текста;

* Приложение декомпрессирует изображение, накладывает строку на изображение, компрессирует результат наложения;

* Изображение-результат возвращается в качестве результата выполнения запроса.

Сервер должен иметь настройку максимального количества одновременно обрабатываемых запросов. При достижения этого количества сервер информирует клиента о невозможности обработки (сервер занят). Сервер не копит очереди запросов.

Отдельно должно быть разработано консольное клиентское приложение, отсылающее указанные при запуске изображение и текст на сервер и записывающее результат в указанную папку. В случае если сервер сообщает о невозможности обработки (сервер занят), клиент делает попытку через задаваемый таймаут. 

## Dependencies

* `libjpeg`

## Linux build

```shell
git submodule update --init
mkdir build && cd build
cmake ..
make
```

## Запуск сервера

```
./jpeg_watermark_server [-a <addr>] [-p <port>] [-t <max_jobs>]

  -a, --addr arg   Server IPv4 address (default: 127.0.0.1)
  -p, --port arg   Port (default: 9001)
  -L, --limit arg  Maximum job limit (default: 0)
      --help       Print help

```

## Запуск клиента

```
./jpeg_watermark_clinet -i <image> -T <text> [-o <output_image>] [-a <addr>] [-p <port>] [-t <timeout>] [-c client_count]

  -a, --addr arg     Server IPv4 address (default: 127.0.0.1)
  -p, --port arg     Server port (default: 9001)
  -i, --image arg    Source image path
  -T, --text arg     Text for drawing
  -t, --timeout arg  Server reconnect interval (default: 5)
  -c, --clients arg  Clients count (default: 1)
  -o, --output arg   Image path for result (default: output.jpg)
      --help         Print help

```



