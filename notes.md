# Retos nivel 4

- [ ] Manejador de la señal **SIGINT**
- [ ] Manejador de la señal **SIGCHLD**

## Señales a tener en cuenta

- **SIGINT**: CTRL + C, provoca que el proceso hijo muera
- **SIGCHLD**: Administra la muerte de un proceso hijo. Lo llamaremos *reaper()*
- **SIGTSTP**: CTRL + Z, .
