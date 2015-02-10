#!/bin/bash
rose='\e[1;31m'
bleu='\e[1;34m'
neutre='\e[0;m'

# Affichage de la derniére connexion local

heure=$(sudo last | grep wtmp | awk -F" " '{print $6}')
date=$(sudo last | grep wtmp | awk -F" " '{print $3" "$4" " $5" " $7" " }')
echo -ne "${rose}La date de la derniére connexion local est:${bleu} $date \n"
echo -ne "${rose}l'heure de La derniére connexion local est:${bleu} $heure\n"
echo -ne "${neutre}"

