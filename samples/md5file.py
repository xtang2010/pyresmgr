import sys, os, argparse

def main():
    parse = argparse.ArgumentParser(description="Send a file to /dev/md5 and get it's md5 sinature back")
    parse.add_argument("filename", help="The file to calculate")

    args = parse.parse_args()

    if (os.path.isfile(args.filename) is None):
        print(f"File {args.filename} does not exist or is not a file.")
        return

    md5 = open("/dev/md5", "rb+")

    with open(args.filename, "rb") as fp:
        while True:
            chunk = fp.read(4096)
            if not chunk:
                break
            md5.write(chunk)

    md5_sig = md5.read(1024)
    print(f"{md5_sig.decode('utf-8')}  {args.filename}")
    md5.close()

    return

if __name__ == "__main__" :
    main()