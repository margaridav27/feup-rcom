APPLICATION LAYER - CRIAR PACOTE
     receber nome do ficheiro
     abrir porta serie
     enviar pacote com nome e tamanho do ficheiro
     ler ficheiro  
     llwrite dos varios fragmentos do ficheiro - cada pacote vai ter o offset da parte do fiheiro onde estamos
     fechar porta serie
     llclose
    --------------------------------------------
     receber nome de ficheiro
     - como é que o recetor sabe que acabou?
     vai lendo e escrevendo no ficheiro

PROTOCOL - CRIAR FRAME

-- ESTABELECIMENTO
    - emissor: manda set
    - recetor: manda UA

-- TRANSFERÊNCIA DE DADOS

EMISSOR:
    - construir a frame
    - stuffing
    - enviar
    - esperar     | timeout - reenviar
                  | RR - envar próxima frame
                  | REJ - reenviar
    - unset

RECETOR:
    - receber a frame
    - verificação do cabeçalho | if erro - descartada
                               | else - processamento
    - verificar se é duplicado | if not - campo de dados aceite e passado à aplicação, responder com RR
                               | else - descartado, responder com RR
    - distuffing
    - verificar BCC2 | if erro - campo de dados descartado mas o campo de controlo pode ser usado para desencadear uma ação adequada (REJ se for um novo frame, RR se for duplicado)
                     | else - mandar para a API & responder com RR


-- API 
                                
    emissor: open [llwrite..llwrite] close
                                                ---> application layer
    recetor: open [llread..llread] close
                     |
                     |
                     v
                physical layer
               write, read, RRs

