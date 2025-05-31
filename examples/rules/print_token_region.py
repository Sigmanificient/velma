import vera

if __name__ == "__main__":
    for file in vera.getSourceFileNames():
        tokens = vera.getTokens(file, 1, 0, 1, 40, [])
        print("->", [t.value for t in tokens])
