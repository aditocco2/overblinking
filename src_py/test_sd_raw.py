def main():
    write_data_raw()
    read_data_raw()

def write_data_raw():
    data = b"If you are reading this, it worked"
    data += b"\x00" * (512 - len(data))
    with open("\\\\.\\D:", "wb") as f:
        size = f.write(data)
        print(size)

def read_data_raw():
    with open("\\\\.\\D:", "rb") as f:
        text = f.read(512)
        print(text)

if __name__ == "__main__":
    main()