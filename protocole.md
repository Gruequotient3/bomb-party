# Bomb party :

## Règle :
- Chaque joueur doit entrer un mot sur une ligne qui comprend la suite de lettre indiquée.
- Si le mot est incorrect (->mot non trouvé dans dictionnaire), reste sur joueur.
- La bombe commence avec un temps entre 10 et 30 secondes.
- Si un des joueurs trouve un mot et que le temps est en dessous du temps minimum (5 secondes par défaut), le temps est reset à ce minimum.


## Tâche :
- Créer le serveur
- Connecter les joueurs (min 2, max 16) et choisir son pseudo.
- Annoncer les arrivées et départs d'un joueur.
- A tour de rôle, le serveur envoi une suite de lettre à tous les joueurs et seulement le joueur concerné doit y répondre par un mot.
- Si un joueur perd, n'a plus la possibilité de jouer mais peut toujours regarder la partie.

### Serveur :
- Accepter tous les joueurs.
	Si au moins 2 joueurs, celui qui a créé le serveur peut y lancer la partie.
- Initialisation de la bombe entre 10 et 30 secondes.
- Initialisation d'une liste de suite de mot à chercher, au moins 30.
- Initialisation de chaque joueur avec 3 vies.
- La partie commence avec celui qui a rejoint en dernier.
- Le serveur envoi la suite de lettre à tous les joueurs. Dans le paquet doit être précisé le joueur qui doit jouer.
- Côté client, si le paquet reçu contient le nom du joueur actuel, alors c'est à lui/elle de jouer. (ex: Joueur X, Joueur Y dans la partie. Les deux joueurs reçoivent le paquet contenant "le" (suite de lettre) et "X".)
	
- Le serveur reçoit le paquet envoyé par un des joueurs. Vérification si le mot existe dans le dictionnaire, si oui, passe au prochain joueur avec une nouvelle suite de lettre.
- Pendant tout ce temps, la bombe décremente chaque seconde.
- Quand elle atteint 5 secondes, si le joueur envoi un mot correct, le timer sera reset à 10 secondes.
- Si la bombe explose pendant qu'on attend un joueur pour son mot, une vie sera retiré. Si 0 vie, ce joueur sera éliminé. 
- Il ne pourra plus envoyer de mot mais pourra toujours voir la partie.
- La partie est terminé quand il ne reste plus qu'un seul joueur.


## Protocole :

### Soucis côté serveur
```
srv : ERROR 00
```

### Commandes invalides :
```
srv : ERROR 10	// argument invalide
srv : ERROR 11	// commande invalide
```

### Connexion d'un client : envoi des joueurs connectés au début et si déconnexion d'un client :
```
srv : PLYRS <liste joueurs> // liste des pseudos des joueurs séparés par "-". ex : PLYRS [joueur1-joueur2-joueur3]

```

### Identification :
```
clt : *

srv : ERROR 01	        // identification requise
srv : ERROR 02	        // partie remplie -> peut seulement voir la partie
```


### pseudo :
```
clt : NAMEP <pseudo> <mode>	//mode = {"J", "S"} Joueur ou Spectateur

srv : NAMEP <pseudo> <mode>	// nouveau joueur connecté (tout le monde)

srv : ERROR 21	        // Pseudo non autorisé
srv : ERROR 22	        // Pseudo déjà pris
srv : ERROR 23	        // Mode non reconnu
```
### Démarrage :
```
clt : START

srv : START	// Partie commence pour tout le monde

srv : ERROR 31	        // Pas assez de joueurs (min 2)
srv : ERROR 32	        // Seulement premier joueur peut lancer la partie
```

### Envoi de suite de lettres aux joueurs:
```
srv : ROUND <lettres> <joueur>	// pour tout le monde
```

### Envoi d'un mot :
```
clt : SENDW <mot>

srv : SENDW <mot> <correct> // correct = {"I", "C"} (tout le monde)

srv : ERROR 33	        // Temps écoulé 
srv : ERROR 34          // paramêtre correct invalide
srv : ERROR 35          // Pas le tour du joueur
srv : ERROR 36	        // Pas joueur (spectateur).
```

### Mort d'un joueur (vie = 0)
```
clt : DEADP

srv : DEADP <joueur>	// (tout le monde)
srv : ERROR 36	        // Pas joueur (spectateur).
```

### Fin de Partie (1 seul joueur restant):
```
srv : GOVER <joueur> (tout le monde)
```

### Verification Connectivité : avant la partie
```
srv: ALIVE // Tout les 3 secondes à tout le monde.

clt: ALIVE
```


### Verification Connectivité : durant la partie
```
srv: ALIVE // Au client silencieux pendant leur tour

clt: ALIVE
```

## Format
### Pseudo 
    [a-zA-Z0-9]{1, 10}



