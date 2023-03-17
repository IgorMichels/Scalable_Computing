n = 300
with open('shakespeare.txt', 'r') as f: text = f.readlines()
text = ''.join(text)

with open('shakespeare_extended.txt', 'w') as f:
    for i in range(n):
        f.write(text)