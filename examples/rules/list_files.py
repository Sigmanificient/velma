import vera

if __name__ == "__main__":
    print(list(vera.getSourceFileNames()))
    print("reversed!", list(reversed(vera.getSourceFileNames())))
