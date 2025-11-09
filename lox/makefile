run:
	@mvn clean package -q
	@java -jar ./target/lox-1.0.jar $(filter-out $@,$(MAKECMDGOALS))
build:
	@mvn clean package
clean:
	@mvn clean

tool:
	@java ./src/main/java/com/craftinginterpreters/tool/GenerateAst.java $(filter-out $@,$(MAKECMDGOALS))