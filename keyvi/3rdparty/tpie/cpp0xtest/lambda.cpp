int main() {
  int x=0;
  return [&]{return x;}();
}
