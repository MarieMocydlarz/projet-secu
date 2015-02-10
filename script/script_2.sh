#!/bin/bash
rose='\e[1;31m'
bleu='\e[1;34m'
neutre='\e[0;m'

# Liste des utilisateurs
echo -ne "\n ${rose}Les utilisateurs humains sont:${bleu}\n"
cat /etc/passwd | grep bash | awk -F":" '{print $1}'
echo -ne "${neutre}"

