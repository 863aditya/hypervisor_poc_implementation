FROM postgres:15-alpine

# Default database credentials (can be overridden at `docker run` time)
ENV POSTGRES_USER=hypervisor_user
ENV POSTGRES_PASSWORD=hypervisor_pass
ENV POSTGRES_DB=hypervisor_db


EXPOSE 5432

