(define-module (demo patterns)
  #:use-module (oop goops)
  #:use-module (gnumaku generics)
  #:use-module (gnumaku core)
  #:use-module (gnumaku math)
  #:use-module (gnumaku primitives)
  #:use-module (gnumaku coroutine)
  #:use-module (gnumaku scene-graph)
  #:use-module (demo actor)
  #:use-module (demo level)
  #:export (spiral1 spiral2 polar-rose fire-at-player))

(define (clamp n min-n max-n)
  (max min-n (min max-n n)))

(define-coroutine (homing-bullet bullet target)
  (let ((direction (rad2deg (atan (- (y target) (bullet-y bullet))
                                  (- (x target) (bullet-x bullet))))))
    (set-bullet-movement bullet 5 direction 0 0))
  (bullet-wait bullet 60)
  (homing-bullet bullet target))

(define-coroutine (sine-wave bullet)
  (define angle-step 20)
  
  (define (step angle)
    (set-bullet-direction bullet (+ (bullet-direction bullet) (* 8 (sin-deg angle))))
    (bullet-wait bullet 4)
    (step (+ angle angle-step)))

  (step 0))

(define-coroutine (explode bullet system delay count speed angle-var)
  (define step (/ 360 count))
  
  (define (emit i)
    (let ((direction (+ (* i step) (random angle-var))))
      (emit-simple-bullet system (bullet-x bullet) (bullet-y bullet)
                          speed direction 0)))
  
  (bullet-wait bullet delay)
  (kill-bullet bullet)
  (repeat count emit))

(define-coroutine (spiral1 actor)
  (define step (/ 360 15))
  
  (define (spiral angle)
    (let ((system (bullet-system actor))
          (x (x actor))
          (y (y actor))
          (player (player (level actor))))
      (emit-script-bullet system x y 0
                          (lambda (bullet)
                            (set-bullet-movement bullet 4 angle 0 0)
                            (explode bullet system 60 8 2 30))))
                            ;;(sine-wave bullet))))
    (wait actor 10)
    (spiral (+ angle step)))

  (spiral 0))

(define-coroutine (spiral2 actor)
  (let loop ((angle 0))
    (emit-bullet (bullet-system actor) (x actor) (y actor) 2 angle
                 .01 2 0 'medium-blue)
    (wait actor 2)
    (loop (+ angle 10))))
