# Notre décodeur JPEG à nous

Bienvenue sur la page d'accueil de _votre_ projet JPEG, un grand espace de liberté, sous le regard bienveillant de vos enseignants préférés.
Le sujet sera disponible dès le lundi 5 mai à l'adresse suivante : [https://formationc.pages.ensimag.fr/projet/jpeg/jpeg/](https://formationc.pages.ensimag.fr/projet/jpeg/jpeg/).

Comme indiqué lors de l'amphi de présentation, vous devrez organiser un point d'étape avec vos enseignants pour valider cette architecture logicielle.
Cette page d'accueil servira de base à cette discussion. En pratique, vous pouvez reprendre son contenu comme bon vous semble, mais elle devra au moins comporter les infos suivantes :

1. des informations sur le découpage des fonctionnalités du projet en modules, en spécifiant les données en entrée et sortie de chaque étape ;
2. (au moins) un dessin des structures de données de votre projet (format libre, ça peut être une photo d'un dessin manuscrit par exemple) ;
3. une répartition des tâches au sein de votre équipe de développement, comportant une estimation du temps consacré à chacune d'elle (là encore, format libre, du truc cracra fait à la main, au joli Gantt chart).

Rajouter **régulièrement** des informations sur l'avancement de votre projet est aussi **une très bonne idée** (prendre 10 min tous les trois chaque matin pour résumer ce qui a été fait la veille, établir un plan d'action pour la journée qui commence et reporter tout ça ici, par exemple).

# Planning prévisionnel 

| Version | Nom de code   | Caractéristiques | Temps estimé |
| :---:   | :---   	  | :--- | :--- |
| 1 	  | Invader 	  | Décodeur d'images 8x8 en niveaux de gris | J+ 4 |
| 2 	  | Noir et blanc | Extension à des images grises comportant plusieurs blocs | J + 6 |
| 3 	  | Couleur 	  | Extension à des images en couleur | J+8 |
| 4	  | Sous-ech 	  | Extension avec des images avec sous-échantionnage | J +10 |

# Droit dépôt git

Si vous avez besoin de droit supplémentaire sur votre dépôt git, venez voir les profs au premier étage du bâtiment E.


# Liens utiles

- Bien former ses messages de commits : [https://www.conventionalcommits.org/en/v1.0.0/](https://www.conventionalcommits.org/en/v1.0.0/) ;
- Problème relationnel au sein du groupe ? Contactez [Pascal](https://fr.wikipedia.org/wiki/Pascal,_le_grand_fr%C3%A8re) !
- Besoin de prendre l'air ? Le [Mont Rachais](https://fr.wikipedia.org/wiki/Mont_Rachais) est accessible à pieds depuis le bâtiment E !
- Un peu juste sur le projet à quelques heures de la deadline ? Le [Montrachet](https://www.vinatis.com/achat-vin-puligny-montrachet) peut faire passer l'envie à certains de vos profs de vous mettre une tôle !

Début de projet :5 Mai 

Au début du projet , nous avons consacré beaucoup de temps à comprendre et assimiler le sujet, 
ce qui s’est révélé précieux pour bien cerner la structure générale du décodeur et réfléchir au découpage modulaire du projet. 
Cette phase de réflexion nous a permis d’élaborer une architecture cohérente.

Étape suivante : implémentation 6 Mai - 21 Mai

Une fois cette première étape franchie, nous sommes passés à l’implémentation. 
Après nous être mis d’accord sur les structures de données communes, nous avons décidé de répartir les modules entre les membres du groupe.
Chaque membre a pris en charge un module à développer, tout en informant régulièrement les autres de son avancement,
afin de garantir une bonne coordination et de faciliter l’intégration future.
##  Répartition des tâches (Objectifs + Code écrit)

| Objectif                               | Membre(s) responsable(s) | Fichier(s) concerné(s) |
|----------------------------------------|--------------------------|------------------------|
| ** Analyse et structuration du projet** | Wala, Hiba, Salma       | (Discussion & plan)  |
| ** Parsing des métadonnées JPEG****     | Wala                    | `parser.c`            |
| ** Implémentation Huffman**             | Hiba                        | `huffman.c`           |
| ** Extraction des données compressées** | Wala                        | `bitreader.c`         |
| ** Reconstruction des blocs MCU**       | Wala                        | `mcu.c`               |
| ** Quantification inverse et Zigzag** | Salma                         | `quantification.c`, `zigzag.c` |
| ** Transformée en cosinus discrète inverse (IDCT)** | Salma           | `idct.c`              |
| ** Conversion couleur YCbCr → RGB** | Wala                            | `ycbcr2rgb.c`         |
| ** Upsampling des blocs Cb/Cr** | Hiba                                 | `upsampling.c`        |
| ** Génération du fichier image final (PPM)** | Hiba                    | `ecriture.c`          |
| ** Tests** | Toutes                          | (Tests collectifs sur differnts images )    |
| ** débogage et optimisation                  | Hiba                    |  `main.c` 

** voici un schéma  qui inclut les entrées et sorties précises de chaque module.
```
                      ┌───────────────────────┐
                      │       parser.c        │
                      │ - Lire entête JPEG    │
                      │ - Stocke dans         │
                      │   Parsed_file         │
                      └──────┬────────────────┘
                             │
                             ▼
                      ┌───────────────────────┐
                      │     bitreader.c       │
                      │ - Gère flux binaire   │
                      │ - Crée BitStream      │
                      └──────┬────────────────┘
                             │
                             ▼
                      ┌───────────────────────┐
                      │      huffman.c        │
                      │ - Arbre Huffman       │
                      │ - Décode Huffman      │
                      │ Entrée : BitStream    │
                      │ Sortie : Coefficients │
                      └──────┬────────────────┘
                             │
                             ▼
                    ┌─────────────────────────┐
                    │         mcu.c           │
                    │ - Extraction MCU        │
                    │ - Decode DC et AC       │
                    │ Entrée : Huffman        │
                    │ Sortie : Blocs 8x8      │
                    └──────┬──────────────────┘
                           │
                           ▼
                    ┌──────────────────────────┐
                    │  quantification.c        │
                    │ - Applique Quantif. Inv. │
                    │ Entrée : Blocs 8x8       │
                    │ + Table qtable[64]       │
                    │ Sortie : Coefficients    │
                    └──────┬────────────────── ┘
                           │
                           ▼
                    ┌──────────────────────────┐
                    │      zigzag.c            │
                    │ - Réordonne coefficients │
                    │ - Zigzag inverse         │
                    │ Entrée : Vecteur 64      │
                    │ Sortie : Matrice 8x8     │
                    └──────┬────────────────── ┘
                           │
                           ▼
                    ┌──────────────────────────┐
                    │       idct.c             │
                    │ - Transformation IDCT    │
                    │ - Convertit en spatial   │
                    │ Entrée : Matrice 8x8     │
                    │ Sortie : Bloc pixels     │
                    └──────┬────────────────── ┘
                           │
                           ▼
                    ┌──────────────────────────┐
                    │     upsampling.c         │
                    │ - Mise à l’échelle       │
                    │ - Ajuste Cb/Cr           │
                    └──────┬──────────────────┘
                           │
                           ▼
                    ┌──────────────────────────┐
                    │      ycbcr2rgb.c         │
                    │ - Conversion couleur     │
                    │ - YCbCr → RGB            │
                    └──────┬──────────────────┘
                           │
                           ▼
                    ┌──────────────────────────┐
                    │      ecriture.c          │
                    │ - Écrit fichier PPM      │
                    │ Sortie : Image .ppm      │
                    └──────────────────────────┘
```
```
 Description des modules du décodeur JPEG
1️⃣ parser.c — Lecture du fichier JPEG

 Ce module analyse l’en-tête du fichier JPEG, extrait les métadonnées et stocke les informations essentielles (dimensions, tables de Huffman, quantification, etc.) dans Parsed_file.  Il détecte les marqueurs JPEG (SOI, DQT, SOS, etc.) et prépare les données pour les étapes suivantes du décodage.
2️⃣ bitreader.c — Gestion du flux binaire

 Il transforme le fichier JPEG en flux de bits (BitStream).  Ce flux est ensuite utilisé pour extraire les données compressées nécessaires au déchiffrement Huffman.
3️⃣ huffman.c — Compression et décompression Huffman

 Ce module construit un arbre Huffman à partir des tables de Huffman extraites.  Il décode les coefficients DC et AC pour récupérer les valeurs quantifiées à partir du flux binaire.  La structure struct tab_huffman est utilisée pour représenter l’arbre Huffman binaire.

 Lien avec mcu.c → huffman.c ne fait que décrypter les coefficients Huffman, tandis que mcu.c s’occupe du décodage des blocs JPEG en utilisant ces valeurs.
4️⃣ mcu.c — Reconstruction des blocs MCU (8x8)

 Ce module décompresse les coefficients Huffman et reconstruit les blocs 8x8 (MCU - Minimum Coding Unit).  Chaque bloc est stocké sous forme de vecteur (Block) de 64 éléments.  Il applique la prédiction DC et organise les blocs pour le traitement fréquentiel.
5️⃣ quantification.c — Quantification inverse

 Cette étape annule la compression JPEG en multipliant chaque coefficient du bloc par la table de quantification.  Elle permet de récupérer les valeurs brutes avant compression.
6️⃣ zigzag.c — Réorganisation des coefficients

 Ce module restructure les coefficients quantifiés en une matrice 8x8, selon le parcours Zigzag inverse.  Il rétablit l'ordre original des fréquences avant de passer à la transformée IDCT.
7️⃣ idct.c — Transformation IDCT

 La Transformée en Cosinus Discrète Inverse (IDCT) est appliquée pour convertir les données fréquentielles en valeurs de pixels réels.  Cette étape est essentielle pour obtenir l’image reconstruite.
8️⃣ upsampling.c — Ajustement des couleurs sous-échantillonnées

 Si le fichier JPEG utilise un sous-échantillonnage chromatique, ce module met à l’échelle les blocs Cb et Cr pour qu’ils aient la même résolution que Y.  Cela permet d’obtenir une image sans perte d’informations colorimétriques.
9️⃣ ycbcr2rgb.c — Conversion YCbCr → RGB

 Ce module convertit les couleurs YCbCr en valeurs RGB adaptées à l’affichage.  Chaque pixel est recalculé pour obtenir ses vraies couleurs, prêtes à être affichées.
🔟 ecriture.c — Génération du fichier PPM

 Une fois l’image reconstruite, ce module écrit le fichier PPM contenant l’image finale décompressée.  Il assure une écriture progressive pour un affichage fluide.
 ```
##  Défis rencontrés et outils utilisés

Dans le cadre du développement de notre décodeur JPEG, plusieurs défis techniques ont été rencontrés. L’un des plus complexes fut la compréhension et l’implémentation de l’algorithme de décompression Huffman, qui joue un rôle essentiel dans la transformation des flux binaires compressés en coefficients exploitables. La gestion des MCU s’est aussi révélée critique, car c’est là que toutes les données décompressées sont assemblées avant d’être reconstruites en image. Nous avons également dû affiner le processus de quantification inverse, zigzag inverse et transformée IDCT, garantissant que les coefficients obtenus restituent une image fidèle après décompression.

Pour surmonter ces défis, nous avons utilisé plusieurs outils et méthodes.Valgrind nous a aidés à détecter d’éventuelles fuites de mémoire et garantir une gestion optimale des allocations et libérations dynamiques. Enfin, l’outil blabla nous a permis de visualiser les traces du décodage sur invader.jpeg, ce qui fut essentiel pour vérifier la validité de notre implémentation à chaque étape du pipeline. Grâce à cette approche, nous avons pu progressivement affiner notre décodeur et garantir son bon fonctionnement 
 Approche incrémentale du développement

Notre objectif a été d'obtenir **rapidement** un décodeur **fonctionnel**, même limité, avant d'ajouter des fonctionnalités plus avancées. Cette méthode nous a permis de :
1️⃣ **Valider chaque étape individuellement** pour garantir leur bon fonctionnement.
2️⃣ **Détecter et corriger les erreurs tôt** afin d'éviter de bloquer l'intégration finale.
3️⃣ **Tester avec des images de complexité croissante**, en suivant l'ordre suggéré.

Grâce à cette approche, nous avons évité **les risques liés à une implémentation complète dès le début**, et nous avons pu obtenir **une version fonctionnelle à chaque instant**, réduisant le stress lié aux deadlines.