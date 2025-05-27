#!/bin/bash

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
NC="\033[0m"

commands=(
    "ls"
    "ls | wc -l"
    "ls | grep .c"
    "ls | grep .c | wc -l"
    "echo hola mundo | wc -w"
    "cat Makefile | grep gcc"
    "ps aux | grep bash | wc -l"
    "echo uno dos tres | wc -w"
    "/bin/ls | wc -l"
    "comando_inexistente"
    "ls     |     wc -l"
    'ls | grep ".zip"'
    'ls | grep ".c" | wc -l'
    'echo "uno dos tres" | wc -w'
    'echo "hola mundo" | grep hola'
    'ls | grep ".png .zip"'
)

passed=0
total=0

echo "🧪 Ejecutando pruebas para tu shell..."

for cmd in "${commands[@]}"; do
    total=$((total+1))
    echo -e "${YELLOW}→ Ejecutando: ${cmd}${NC}"

    # Ejecutar en bash
    bash_out=$(eval "$cmd")

    # Ejecutar en tu shell
    shell_out=$(echo "$cmd" | ./shell)

    if [ "$bash_out" == "$shell_out" ]; then
        echo -e "${GREEN}✅ PASS:${NC} $cmd"
        passed=$((passed+1))
    else
        echo -e "${RED}❌ FAIL:${NC} $cmd"
        echo "    Bash :  '$bash_out'"
        echo "    Shell:  '$shell_out'"
    fi

    echo "--------------------------------------"
done

echo ""
echo "📊 Resultado: $passed / $total tests pasados"
