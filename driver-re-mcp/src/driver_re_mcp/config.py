from pathlib import Path
from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    """Configuration for Driver RE MCP server"""

    DATABASE_PATH: str = "./data/driver_re.db"
    CHROMADB_PATH: str = "./data/chromadb"
    GHIDRA_HOST: str = "localhost"
    GHIDRA_PORT: int = 8080
    SEMANTIC_SEARCH_THRESHOLD: float = 0.7
    MAX_SEARCH_RESULTS: int = 50

    class Config:
        env_file = ".env"
        env_prefix = "DRIVER_RE_"

    @property
    def database_path_obj(self) -> Path:
        return Path(self.DATABASE_PATH)

    @property
    def chromadb_path_obj(self) -> Path:
        return Path(self.CHROMADB_PATH)

    @property
    def ghidra_url(self) -> str:
        return f"http://{self.GHIDRA_HOST}:{self.GHIDRA_PORT}"

settings = Settings()
