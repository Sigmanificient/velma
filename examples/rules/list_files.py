import vera

if __name__ == "__main__":
    print(list(vera.getSourceFileNames()))

    for file in reversed(vera.getSourceFileNames()):
        print(file, vera.getLineCount(file))
