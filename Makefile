clean:
	rm -rf .pytest_cache/ tests/_pycache__/ src/polygon_tools.egg-info/ build/
	yes | pip3 uninstall polygon_tools || true

install:
	pip3 install --user .

test: clean install
	pytest

wheel:
	rm dist/* || true
	docker build -t polygontools:builder .
	docker run --rm polygontools:builder &
	sleep 30
	bash scripts/build--copy-output.sh
	ls -lah dist/

