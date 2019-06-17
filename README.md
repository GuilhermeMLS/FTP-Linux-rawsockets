# Trabalho 1 de Rede de Computadores I
UFPR - Ciência da Computação
## Alunos:
- Guilherme M. Lopes GRR 20163043
- Leonardo Stefan GRR 20163052

## Makefile:
- *make*
- *make clean*
- *make purge*
- *make remake*

##### Slave:
```
	sudo ./kermit <network_device> s
```
##### Master
```
	sudo ./kermit <network_device> m
```
### Comandos:
- *ls*
- *ls -l*
- *ls -a*
- *put* {arquivo}
- *get* {arquivo}
- *cd* {diretório}
- *R* entra no contexto remoto
- *L* entra no contexto local

### Decisões de projeto:
- Foi utilizado o padrão de tabela para o cálculo do CRC-8;
- Timeout de 1024 tentativas inspirado no protocolo CSMA.

