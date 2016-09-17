FROM ubuntu:16.04
MAINTAINER Ken Thompson "ken@bell-labs.com"
LABEL Description="CSAW 2016 Reverse Polish 💩" VERSION=1.0

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y build-essential socat

#user
RUN adduser --disabled-password --gecos '' ken
RUN chown -R root:ken /home/ken/
RUN chmod 750 /home/ken
RUN touch /home/ken/flag.txt
RUN chown root:ken /home/ken/flag.txt
RUN chmod 440 /home/ken/flag.txt
RUN chmod 740 /usr/bin/top
RUN chmod 740 /bin/ps
RUN chmod 740 /usr/bin/pgrep
RUN export TERM=xterm
WORKDIR /home/ken/

COPY ./build.sh /home/ken
COPY ./flag.txt /home/ken

RUN mkdir /home/ken/src
RUN chown -R root:ken /home/ken/src
RUN chmod 750 /home/ken/src


COPY ./src/ /home/ken/src

RUN ./build.sh

CMD su ken -c "socat TCP-LISTEN:6565,reuseaddr,fork EXEC:/home/ken/rpl"

