# Minishell — Interpréteur de commandes Unix en C

## Présentation

Minishell est un interpréteur de commandes simplifié, développé en langage C dans le cadre d'un projet universitaire.

L'objectif de ce projet est de comprendre le fonctionnement d'un shell Unix et de mettre en pratique les concepts de programmation système, notamment la gestion des processus, l'exécution de commandes et la manipulation des variables d'environnement.

## Fonctionnalités

- Exécution de commandes système.
- Gestion des commandes internes : `cd`, `pwd`, `exit`, `export` et `unset`.
- Création et gestion des processus avec `fork()`, `execvp()` et `waitpid()`.
- Analyse des commandes saisies par l'utilisateur.
- Implémentation des opérateurs et des redirections (fonctionnalités nécessitant des tests complémentaires).

## Technologies utilisées

- **Langage :** C
- **Environnement :** Unix / Linux
- **Compilateur :** GCC
- **Compilation :** Makefile
- **Gestion de versions :** Git et GitHub

## Installation et exécution

### 1. Cloner le projet

```bash
git clone https://github.com/ChaymaeZitouni/minishell-c.git
```

### 2. Accéder au dossier

```bash
cd minishell-c
```

### 3. Compiler le projet

```bash
make
```

### 4. Lancer Minishell

```bash
./minishell
```

## Exemple d'utilisation

```text
$ pwd
/workspaces/minishell-c
$ exit
```

## Compétences mises en pratique

Ce projet m'a permis de développer mes compétences en :

- Programmation système en C.
- Gestion des processus Unix.
- Analyse et exécution de commandes.
- Manipulation des entrées et sorties.
- Compilation et organisation d'un projet C.

## Auteures

Projet universitaire réalisé par :

- **Chaymae Zitouni**
- **Basma Moujane**
