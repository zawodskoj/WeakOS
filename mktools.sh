rm -rf tools
mkdir tools

#собираем тулзы
for toolName in `find src/tools/* -type d -printf "%f\n"`; do
    clang src/tools/${toolName}/*.c --std=c11 -mno-sse -o tools/${toolName} -g
done
