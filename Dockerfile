# Start from the base 
FROM ubuntu:20.04

ARG CELERY_PACKAGE=celery==5.3.0
ARG PSYCOPG2_BINARY=psycopg2-binary==2.9.6

# Install system deps 
RUN apt-get -yqq update
RUN apt-get -yqq install python3-pip python3-dev
RUN apt-get -yqq install wget unzip
RUN mkdir -p /CE

# Clone the repository
ADD ./CE /Scalable_Computing/Project_A2

WORKDIR /CE/

# Fetch apps 
RUN pip install ${CELERY_PACKAGE} ${PSYCOPG2_BINARY}

ENV POSTGRESQL_HOST=db-highway-dashboard.c7mkrwgemklj.us-east-1.rds.amazonaws.com
ENV POSTGRESQL_USER=postgres
ENV POSTGRESQL_PASSWORD=postgrespassword
ENV BROKER_URL=

# In reality, you possibly should mitigate the loggings
CMD ["celery", "-A", "communication", "worker", "-l", "INFO"]