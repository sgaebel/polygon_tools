clean:
	rm -rf .pytest_cache/ tests/_pycache__/ src/polygon_tools.egg-info/ build/
	yes | pip3 uninstall polygon_tools || true

install:
	pip3 install --user .

test: clean install
	pytest
